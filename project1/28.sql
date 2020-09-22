select t.name, avg(level) 
from CatchedPokemon c, Trainer t
where c.owner_id = t.id and c.pid in(select id
                                        from Pokemon
                                        where type = 'Normal' or type = 'Electric')
group by t.id
order by avg(level)