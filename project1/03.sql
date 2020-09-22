select avg(c.level) as 'average_level'
from CatchedPokemon c, Pokemon p
where c.pid = p.id and p.type = 'Electric'
      and c.owner_id in(select t.id
                    from Trainer t
                    where t.hometown = 'Sangnok City') ;
                    