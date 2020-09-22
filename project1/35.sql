select name
from Pokemon
where id in(select before_id
            from Evolution
            where before_id > after_id)
order by name;            